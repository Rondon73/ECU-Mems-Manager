export type BotConfig = {
  id: string;
  name: string;
  role: string;
  avatar: string;
  model: string;
};

export const DEFAULT_BOTS: BotConfig[] = [
  {
    id: "strategist",
    name: "Ava Vision",
    role: "Strategist",
    avatar: "🧠",
    model: "claude-sonnet-4-5",
  },
  {
    id: "project-manager",
    name: "Max Flow",
    role: "Project Manager",
    avatar: "📋",
    model: "gpt-4.1",
  },
  {
    id: "copywriter",
    name: "Luna Spark",
    role: "Copywriter",
    avatar: "✨",
    model: "gpt-4.1",
  },
  {
    id: "designer",
    name: "Kai Frame",
    role: "Designer",
    avatar: "🎨",
    model: "gpt-image-1",
  },
  {
    id: "analyst",
    name: "Nora Pulse",
    role: "Data Analyst",
    avatar: "📊",
    model: "gpt-4.1",
  },
  {
    id: "qa",
    name: "Eli Guard",
    role: "QA Reviewer",
    avatar: "🛡️",
    model: "claude-sonnet-4-5",
  },
];

export const MODEL_OPTIONS = [
  "gpt-4.1",
  "gpt-4.1-mini",
  "gpt-5-mini",
  "gpt-image-1",
  "claude-sonnet-4-5",
  "claude-haiku-4-5",
  "gemini-1.5-pro",
];
